"""Compile Nano variants and execute the actual AVR binaries in avr8js."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cli', default='arduino-cli')
    parser.add_argument('--config')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    build = root / '.build'
    cli = [args.cli] + (['--config-file', args.config] if args.config else [])

    def compile_sketch(name, sketch, flags='', cpu='atmega328'):
        command = cli + ['compile', '--fqbn', f'arduino:avr:nano:cpu={cpu}',
                         '--warnings', 'all', '--output-dir', str(build / name)]
        if flags:
            command += ['--build-property', 'compiler.cpp.extra_flags=' + flags]
        print(f'Compiling {name}', flush=True)
        subprocess.run(command + [str(sketch)], check=True)

    compile_sketch('demo', root)
    compile_sketch('lcd-demo', root, '-DST_PIN_MAPPING_CONFIRMED=1 -DST_LCD_ENABLED=1 -DST_RELAY_ENABLED=1')
    full = ('-DST_DEMO_MODE=0 -DST_PIN_MAPPING_CONFIRMED=1 -DST_LCD_ENABLED=1 '
            '-DST_RELAY_ENABLED=1 -DST_LEDS_ENABLED=1 -DST_TANK_ENABLED=1 ')
    compile_sketch('dht22', root, full + '-DST_DHT_MODEL=22')
    compile_sketch('dht11-old', root, full + '-DST_DHT_MODEL=11', 'atmega328old')
    compile_sketch('real-headless', root, '-DST_DEMO_MODE=0 -DST_PIN_MAPPING_CONFIRMED=1')

    # Stage the maintained headers beside the test sketch. This works with
    # Arduino's build staging on Windows paths containing spaces as well.
    with tempfile.TemporaryDirectory(prefix='smart-tomchi-') as temporary:
        sketch = Path(temporary) / 'ControllerTests'
        sketch.mkdir()
        shutil.copy2(root / 'tests' / 'ControllerTests' / 'ControllerTests.ino', sketch)
        for header in root.glob('*.h'):
            shutil.copy2(header, sketch)
        compile_sketch('unit', sketch, '-DST_DEMO_MODE=0 -DST_PIN_MAPPING_CONFIRMED=1 -DST_RELAY_ENABLED=1')

    for name, filename, mode in [('unit', 'ControllerTests', 'unit'),
                                 ('demo', 'SmartTomchi', 'demo'),
                                 ('lcd-demo', 'SmartTomchi', 'lcd-demo')]:
        subprocess.run(['node', str(root / 'tests' / 'simulate.cjs'),
                        str(build / name / f'{filename}.ino.hex'), mode], check=True,
                       env=os.environ.copy())


if __name__ == '__main__':
    main()
