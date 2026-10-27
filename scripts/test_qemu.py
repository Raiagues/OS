#!/usr/bin/env python3
"""Boot real RV64 ELFs under QEMU and verify observable kernel behavior."""
import argparse
import os
from pathlib import Path
import re
import selectors
import subprocess
import sys
import time


def check_elf(readelf, kernel):
    header = subprocess.check_output([readelf, '-h', str(kernel)], text=True)
    for expected in ('ELF64', 'RISC-V', '0x80000000'):
        if expected not in header:
            raise AssertionError(f'{kernel}: unexpected ELF header (missing {expected})')


def run_kernel(qemu, kernel, final_marker, timeout, log_path):
    command = [qemu, '-machine', 'virt', '-bios', 'none', '-kernel', str(kernel),
               '-m', '128M', '-smp', '1', '-display', 'none', '-monitor', 'none',
               '-serial', 'stdio', '-no-reboot']
    started = time.monotonic()
    deadline = started + timeout
    output = bytearray()
    completed = False
    process = subprocess.Popen(command, stdin=subprocess.DEVNULL,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    selector = selectors.DefaultSelector()
    selector.register(process.stdout, selectors.EVENT_READ)
    try:
        while time.monotonic() < deadline:
            for key, _ in selector.select(timeout=0.1):
                chunk = os.read(key.fd, 65536)
                if chunk:
                    output.extend(chunk)
                else:
                    selector.unregister(key.fileobj)
            if final_marker.encode() in output.replace(b"\r", b""):
                completed = True
                break
            if process.poll() is not None:
                break
    finally:
        selector.close()
        if process.poll() is None:
            process.terminate()
        try:
            remaining, _ = process.communicate(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            remaining, _ = process.communicate()
        output.extend(remaining)
    text = output.decode('utf-8', errors='replace').replace('\r', '')
    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.write_text(text, encoding='utf-8')
    if not completed:
        raise AssertionError(f'{kernel}: did not reach {final_marker!r} within {timeout}s\n{text}')
    if 'PANIC:' in text or 'SELFTEST: FAIL' in text:
        raise AssertionError(f'{kernel}: kernel reported a failure\n{text}')
    print(f'PASS {kernel}: completed in {time.monotonic() - started:.1f}s; log: {log_path}')
    return text


def check_demo(text):
    if 'OS / RISC-V RV64 / QEMU virt' not in text:
        raise AssertionError('Boot banner missing')
    for pid, delay in ((1, 2), (2, 3), (3, 4)):
        created = f'Processus: proc{pid} cree avec pid {pid}'
        if text.count(created) != 1:
            raise AssertionError(f'Process {pid} was not created exactly once')
        samples = re.findall(r'\[temps = (\d+) s\] processus proc' + str(pid)
                             + r' pid = ' + str(pid) + r'\b', text)
        if len(samples) != 2 or int(samples[1]) - int(samples[0]) < delay:
            raise AssertionError(f'Process {pid}: unexpected sleep samples {samples}')
        for prefix in ('Debut', 'Fin'):
            if text.count(f'{prefix} du sommeil de {delay} s du processus {pid}') != 2:
                raise AssertionError(f'Process {pid}: expected two {prefix.lower()} sleep events')
        if text.count(f'Fin du processus {pid}\n') != 1:
            raise AssertionError(f'Process {pid}: missing or repeated termination')


def check_selftest(text):
    markers = ['SELFTEST: START', 'SELFTEST: FORMAT PASS', 'SELFTEST: PREEMPT PASS',
               'SELFTEST: SLEEP PASS', 'SELFTEST: EXIT PASS', 'SELFTEST: ALL PASS']
    for marker in markers:
        if text.count(marker) != 1:
            raise AssertionError(f'Self-test expected exactly one {marker!r}')
    if 'SELFTEST: FAIL' in text:
        raise AssertionError('Self-test failure')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', default='qemu-system-riscv64')
    parser.add_argument('--readelf', default='riscv64-unknown-elf-readelf')
    parser.add_argument('--demo', type=Path, default=Path('build/virt/kernel.elf'))
    parser.add_argument('--selftest', type=Path, default=Path('build/virt-selftest/kernel.elf'))
    parser.add_argument('--timeout', type=float, default=25,
                        help='Maximum wall-clock seconds for each kernel')
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error('--timeout must be positive')
    try:
        for kernel in (args.demo, args.selftest):
            check_elf(args.readelf, kernel)
        demo = run_kernel(args.qemu, args.demo, '[OS] DEMO COMPLETE: proc1, proc2, proc3 finished; idle running.\n', args.timeout,
                          args.demo.parent / 'qemu-demo.log')
        check_demo(demo)
        selftest = run_kernel(args.qemu, args.selftest, 'SELFTEST: ALL PASS\n', args.timeout,
                              args.selftest.parent / 'qemu-selftest.log')
        check_selftest(selftest)
    except (AssertionError, OSError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    print('All QEMU checks passed: boot, round-robin, sleep/wakeup, termination and timer preemption.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
