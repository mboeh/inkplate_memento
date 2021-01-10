#!/usr/bin/env ruby

require 'uart'

usb = ARGV[0] || '/dev/cu.usbserial-1410'
commands = STDIN.each_line.to_a
$n = 0

def do_cmd(serial, cmd, arg)
  case cmd
  when '@', '<', '>', '_'
    serial.write [cmd.ord, arg.to_i].pack('CC')
  when '$'
    n, text = arg.split(/\s+/, 2)
    serial.write [cmd.ord, n.to_i, text.bytesize].pack('CCC')
    serial.write text
  when '*'
    if arg =~ /^\d+/
      n, typ, arg_n, rest = arg.split(/\s+/, 4)
    else
      n, typ, arg_n, rest = $n, *arg.split(/\s+/, 3)
    end
    serial.write [cmd.ord, n.to_i, typ.ord, arg_n.to_i].pack('CCCC')
    if typ == '$' and not rest.empty?
      do_cmd(serial, '$', "#{arg_n} #{rest}")
    end
    $n = n.to_i + 1
  when '='
    serial.write cmd
  else
    STDERR.puts "Bad command #{cmd} #{arg}"
  end
end

UART.open usb, 115200 do |serial|
  commands.each do |line|
    cmd, arg = line.chomp.split(/\s+/, 2)
    do_cmd(serial, cmd, arg)
  end
end
