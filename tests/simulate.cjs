// Real compiled AVR firmware runs inside avr8js; no copy of application logic.
const fs = require('node:fs');
const assert = require('node:assert/strict');
const avr = require('avr8js');
const [hexPath, mode = 'demo'] = process.argv.slice(2);
assert(hexPath, 'Usage: node tests/simulate.cjs firmware.hex demo|lcd-demo|unit');
const bytes = new Uint8Array(32768);
for (const line of fs.readFileSync(hexPath, 'utf8').trim().split(/\r?\n/)) {
  assert(line.startsWith(':'), 'Intel HEX record');
  const record = Buffer.from(line.slice(1), 'hex');
  assert.equal([...record].reduce((sum, value) => sum + value, 0) & 255, 0, 'HEX checksum');
  if (record[3] === 0) {
    const address = (record[1] << 8) | record[2];
    bytes.set(record.subarray(4, 4 + record[0]), address);
  } else assert([1, 3, 5].includes(record[3]), 'Unsupported extended HEX address');
}
const cpu = new avr.CPU(new Uint16Array(bytes.buffer));
new avr.AVRTimer(cpu, avr.timer0Config);
new avr.AVRTimer(cpu, avr.timer1Config);
new avr.AVRTimer(cpu, avr.timer2Config);
const portB = new avr.AVRIOPort(cpu, avr.portBConfig);
new avr.AVRIOPort(cpu, avr.portCConfig);
new avr.AVRIOPort(cpu, avr.portDConfig);
// Minimal HD44780 receiver: decode actual RS/E/D4..D7 output edges.
// Includes the power-on transition from 8-bit initialization to 4-bit mode.
const ddram = new Uint8Array(128).fill(32);
let fourBit = false, pending = null, address = 0, cgram = false;
portB.addListener((value, previous) => {
  if (mode !== 'lcd-demo' || !(previous & 8) || (value & 8)) return;
  const data = cpu.data[0x2b];
  const nibble = ((data >> 5) & 1) | (((data >> 4) & 1) << 1) |
                 (((data >> 3) & 1) << 2) | (((data >> 2) & 1) << 3);
  if (!fourBit) { if (nibble === 2) fourBit = true; return; }
  if (pending === null) { pending = nibble; return; }
  const byte = (pending << 4) | nibble;
  pending = null;
  if (value & 16) {
    if (!cgram) ddram[address & 127] = byte;
    address = (address + 1) & 127;
  } else if (byte & 128) { address = byte & 127; cgram = false; }
  else if (byte & 64) { address = byte & 63; cgram = true; }
  else if (byte === 1) { ddram.fill(32); address = 0; cgram = false; }
  else if (byte === 2) { address = 0; cgram = false; }
});
new avr.AVREEPROM(cpu, new avr.EEPROMMemoryBackend(1024));
const adc = new avr.AVRADC(cpu, avr.adcConfig);
adc.channelValues[0] = 2.5;
const uart = new avr.AVRUSART(cpu, avr.usart0Config, 16000000);
const lines = [];
let finished = false;
let frameRow = 0;
let decodedRows = 0;
uart.onLineTransmit = (line) => {
  const clean = line.replace(/\r$/, '');
  lines.push(clean);
  if (mode === 'lcd-demo' && clean.startsWith('|')) {
    const offset = frameRow === 0 ? 0 : 64;
    const actual = Array.from(ddram.subarray(offset, offset + 16),
      c => c >= 1 && c <= 5 ? '#' : String.fromCharCode(c)).join('');
    assert.equal(actual, clean.slice(1, -1), `HD44780 row ${frameRow}`);
    frameRow = 1 - frameRow;
    decodedRows++;
  }
  if (mode === 'unit') {
    if (/^(PASS|FAIL|CHECKS=|ALL_TESTS|TESTS_FAILED)/.test(clean)) console.log(clean);
    if (/ALL_TESTS_PASSED|TESTS_FAILED/.test(clean)) finished = true;
  }
};
const limit = 16000000 * (mode === 'unit' ? 12 : 24);
while (cpu.cycles < limit && !finished) {
  avr.avrInstruction(cpu);
  cpu.tick();
}
const output = lines.join('\n');
if (mode === 'unit') {
  assert(output.includes('ALL_TESTS_PASSED'), output || 'No UART output');
  assert(!output.includes('FAIL '), output);
} else {
  for (const label of ['SMART TOMCHI', 'DEMO Namlik:', 'DEMO Havo', 'DEMO Tahlil', "DEMO Sug'orish", 'DEMO Tizim'])
    assert(output.includes(label), `Missing screen: ${label}`);
  const frames = lines.filter(line => line.startsWith('|'));
  assert(frames.length > 10, 'Not enough display frames');
  for (const frame of frames) assert.equal(frame.length, 18, `LCD width: ${frame}`);
  if (mode === 'demo') {
    assert.equal(cpu.data[0x24], 0, 'DDRB');
    assert.equal(cpu.data[0x27], 0, 'DDRC');
    assert.equal(cpu.data[0x2a] & 0xfc, 0, 'DDRD external outputs');
  } else {
    assert(decodedRows > 10, 'Actual HD44780 output was not decoded');
    assert.equal(cpu.data[0x2a] & 0x40, 0, 'DEMO must not activate relay GPIO even if enabled');
  }
  console.log(`PASS ${frames.length / 2} LCD frames, all five pages; decoded LCD rows=${decodedRows}`);
}
console.log(`Simulation passed: ${mode}`);
