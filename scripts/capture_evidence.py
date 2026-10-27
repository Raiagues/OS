#!/usr/bin/env python3
"""Run the real CEP kernel, assert its logs, and capture its framebuffer via QMP.

Requires the course QEMU (machine cep). PNGs contain actual guest pixels;
there is no simulated terminal rendering or image generation in this script.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import socket
import struct
import subprocess
import tempfile
import time
import zlib

ROOT = Path(__file__).resolve().parents[1]
WIDTH, HEIGHT = 1280, 720
FRAMEBUFFER = 0x80000000


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def png_from_framebuffer(raw, destination):
    if len(raw) != WIDTH * HEIGHT * 4:
        raise RuntimeError(f'Unexpected framebuffer length: {len(raw)}')
    # The guest writes 0xAARRGGBB as little-endian 32-bit pixels: B,G,R,A.
    rgb = bytearray(WIDTH * HEIGHT * 3)
    rgb[0::3] = raw[2::4]
    rgb[1::3] = raw[1::4]
    rgb[2::3] = raw[0::4]
    rows = b''.join(b'\0' + rgb[y * WIDTH * 3:(y + 1) * WIDTH * 3]
                    for y in range(HEIGHT))
    def chunk(kind, data):
        return (struct.pack('!I', len(data)) + kind + data
                + struct.pack('!I', zlib.crc32(kind + data) & 0xffffffff))
    destination.write_bytes(b'\x89PNG\r\n\x1a\n'
                           + chunk(b'IHDR', struct.pack('!2I5B', WIDTH, HEIGHT, 8, 2, 0, 0, 0))
                           + chunk(b'IDAT', zlib.compress(rows, 9)) + chunk(b'IEND', b''))


class QMP:
    def __init__(self, path):
        self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.socket.settimeout(10)
        self.socket.connect(str(path))
        self.reader = self.socket.makefile('rb')
        greeting = json.loads(self.reader.readline())
        if 'QMP' not in greeting:
            raise RuntimeError(f'Invalid QMP greeting: {greeting}')
        self.command('qmp_capabilities')

    def command(self, name, arguments=None):
        request = {'execute': name}
        if arguments is not None:
            request['arguments'] = arguments
        self.socket.sendall(json.dumps(request).encode() + b'\n')
        while True:
            line = self.reader.readline()
            if not line:
                raise RuntimeError(f'QEMU disconnected during {name}')
            reply = json.loads(line)
            if 'error' in reply:
                raise RuntimeError(f'QMP {name}: {reply["error"]}')
            if 'return' in reply:
                return reply['return']

    def capture(self, output, temporary):
        self.command('stop')
        dump = temporary / 'framebuffer.bin'
        try:
            self.command('pmemsave', {'val': FRAMEBUFFER, 'size': WIDTH * HEIGHT * 4,
                                     'filename': str(dump)})
            raw = dump.read_bytes()
            png_from_framebuffer(raw, output)
            return raw
        finally:
            self.command('cont')

    def close(self):
        self.reader.close()
        self.socket.close()


def run_capture(args, mode, targets):
    build = ROOT / 'build' / ('cep' if mode == 'processes' else f'cep-{mode}')
    kernel = build / 'kernel.elf'
    command = ['make', 'PLATFORM=cep', f'DEMO={mode}', 'SELFTEST=0', 'all']
    result = subprocess.run(command, cwd=ROOT, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=120)
    if result.returncode:
        raise RuntimeError(result.stdout)
    log = args.output / f'{mode}.txt'
    log.unlink(missing_ok=True)
    with tempfile.TemporaryDirectory(prefix='os-qmp-') as temporary_name:
        temporary = Path(temporary_name)
        qmp_path = temporary / 'monitor.sock'
        stderr_path = temporary / 'qemu.stderr'
        qemu_command = [args.qemu, '-machine', 'cep', '-bios', 'none', '-kernel', str(kernel),
                        '-m', '32M', '-smp', '1', '-display', args.display,
                        '-monitor', 'none', '-serial', f'file:{log}',
                        '-qmp', f'unix:{qmp_path},server=on,wait=off', '-no-reboot']
        with stderr_path.open('wb') as error_file:
            process = subprocess.Popen(qemu_command, stdin=subprocess.DEVNULL,
                                       stdout=subprocess.DEVNULL, stderr=error_file)
            qmp = None
            captures = []
            try:
                deadline = time.monotonic() + args.timeout
                while not qmp_path.exists():
                    if process.poll() is not None or time.monotonic() >= deadline:
                        raise RuntimeError(f'{mode}: QEMU failed: {stderr_path.read_text()}')
                    time.sleep(0.02)
                qmp = QMP(qmp_path)
                for marker, filename, settle in targets:
                    while True:
                        text = log.read_text(errors='replace').replace('\r', '') if log.exists() else ''
                        if 'PANIC:' in text or 'EVIDENCE: FAIL' in text or 'SELFTEST: FAIL' in text:
                            raise RuntimeError(f'{mode}: kernel failure\n{text}')
                        if marker in text:
                            break
                        if process.poll() is not None or time.monotonic() >= deadline:
                            raise RuntimeError(f'{mode}: missing {marker!r}\n{text}\n{stderr_path.read_text()}')
                        time.sleep(0.01)
                    if settle:
                        time.sleep(settle)
                    image = args.output / filename
                    raw = qmp.capture(image, temporary)
                    if mode == 'pixels':
                        # ASCII A is column 33; its first bitmap row has bit 2 set.
                        for row, color in enumerate((0xff0000, 0x00ff00, 0x0000ff, 0xffffffff), 1):
                            actual = struct.unpack_from('<I', raw, (row * 8 * WIDTH + 33 * 8 + 2) * 4)[0]
                            if actual != color:
                                raise RuntimeError(f'ASCII row {row}: expected {color:#x}, got {actual:#x}')
                        paragraph = text.split('\f', 1)[1].split('\nEVIDENCE: PIXELS PASS', 1)[0]
                        if len(paragraph) < 1600 or '\n' in paragraph or '\r' in paragraph:
                            raise RuntimeError('Expected one long paragraph without newline bytes')
                        rows, column = divmod(len(paragraph), 160)
                        cursor = ((6 + rows) * 8 + 7) * WIDTH + column * 8
                        if raw[cursor * 4:(cursor + 8) * 4] != b'\xff' * 32:
                            raise RuntimeError('Cursor did not reach the expected position after wrapping')
                        time.sleep(1)
                        if qmp.capture(temporary / 'steady.png', temporary) != raw:
                            raise RuntimeError('The static example changed after one second')
                    captures.append({'file': filename, 'trigger': marker.strip(),
                                     'settle_seconds': settle, 'sha256': digest(image)})
            finally:
                if qmp is not None:
                    qmp.close()
                if process.poll() is None:
                    process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
        text = log.read_text(errors='replace').replace('\r', '')
        if 'PANIC:' in text or 'EVIDENCE: FAIL' in text:
            raise RuntimeError(f'{mode}: failure after capture\n{text}')
        if mode == 'uart':
            for marker in ('UART_ONLY: sent directly to UART.', 'CONSOLE_MIRROR: screen and UART.'):
                if text.count(marker) != 1:
                    raise RuntimeError(f'UART bytes missing or duplicated: {marker}')
        if mode == 'scroll':
            lines = re.findall(r'(?:^|\f)line (\d+) \|', text, re.M)
            if lines != [f'{n:02d}' for n in range(100)]:
                raise RuntimeError('Scrolling example did not emit the expected 100 lines')
        if mode == 'clock':
            seconds = re.findall(r'EVIDENCE: CLOCK SECOND=(\d+)', text)
            if seconds[:4] != ['0', '1', '2', '3']:
                raise RuntimeError(f'Unexpected clock progression: {seconds}')
        if mode == 'processes':
            # Reuse the behavioral checks of the existing integration test.
            from test_qemu import check_demo
            check_demo(text.replace('QEMU CEP', 'QEMU virt'))
        log.write_text(text)
    print(f'PASS {mode}: {", ".join(x["file"] for x in captures)}', flush=True)
    return {'mode': mode, 'make': ' '.join(command),
            'kernel_sha256': digest(kernel), 'uart_log': log.name,
            'uart_sha256': digest(log), 'captures': captures}


def version(command):
    return subprocess.check_output(command, text=True).splitlines()[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'build/evidence')
    parser.add_argument('--qemu', default=os.environ.get('QEMU', 'qemu-system-riscv64'))
    parser.add_argument('--timeout', type=float, default=30)
    parser.add_argument('--display', choices=['none', 'sdl'], default='none',
                        help='none captures guest VRAM without a desktop; sdl also opens a real window')
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error('--timeout must be positive')
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    report_path = args.output / 'report.json'
    report_path.unlink(missing_ok=True)  # Never keep a stale PASS after a failure.
    machines = subprocess.check_output([args.qemu, '-machine', 'help'], text=True)
    if not re.search(r'^cep\s', machines, re.M):
        parser.error('This capture needs the course QEMU with machine cep.')
    scenarios = [
        ('pixels', [('EVIDENCE: PIXELS PASS\n', 'pixels.png', 0)]),
        ('console', [('EVIDENCE: CONSOLE PASS\n', 'console.png', 0)]),
        ('scroll', [('EVIDENCE: SCROLL PASS written=100 first_visible=12 last_visible=99\n', 'scroll.png', 0)]),
        ('uart', [('EVIDENCE: UART PASS direct_tx=1 console_mirror=1\n', 'uart.png', 0)]),
        ('clock', [('EVIDENCE: CLOCK PASS\n', 'clock.png', 0)]),
        ('processes', [('Debut du sommeil de 4 s du processus 3\n', 'processes-start.png', 1.1),
                       ('[OS] DEMO COMPLETE: proc1, proc2, proc3 finished; idle running.\n',
                        'processes-end.png', 1.1)]),
    ]
    results = [run_capture(args, mode, targets) for mode, targets in scenarios]
    result = subprocess.run(['make', f'QEMU={args.qemu}', 'DEMO=processes', 'test'], cwd=ROOT,
                            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=150)
    if result.returncode:
        raise RuntimeError(result.stdout)
    for original, name in [('build/virt/qemu-demo.log', 'virt-processes.txt'),
                           ('build/virt-selftest/qemu-selftest.log', 'selftest.txt')]:
        (args.output / name).write_bytes((ROOT / original).read_bytes())
    report = {'generated_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'status': 'PASS', 'platform': 'cep', 'display_backend': args.display,
              'capture_method': 'Actual 1280x720 framebuffer at 0x80000000 read with QMP pmemsave; lossless PNG encoding',
              'qemu': version([args.qemu, '--version']),
              'compiler': version([os.environ.get('CROSS_COMPILE', 'riscv64-unknown-elf-') + 'gcc', '--version']),
              'architecture': os.environ.get('RISCV_ARCH', 'rv64ima_zicsr'),
              'sources_sha256': {str(p.relative_to(ROOT)): digest(p)
                                 for p in sorted((ROOT / 'src').iterdir()) if p.is_file()},
              'scenarios': results,
              'integration_test': 'make DEMO=processes test: passed',
              'selftest_sha256': digest(args.output / 'selftest.txt')}
    report_path.write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n')
    print(f'PASS all captures and integration tests. Report: {report_path}')


if __name__ == '__main__':
    main()
