#!/usr/bin/env ruby
require 'yaml'
require 'socket'
require 'colorize'

HOST = 'localhost'
PORT = 3000

def request(method, path, headers, body)
  print "#{method} #{path}   "
  STDOUT.flush

  s = TCPSocket.open HOST, PORT
  s.puts "#{method} #{path} HTTP/1.1\r\n"
  s.puts "Connection: close\r\n"
  headers.each do |k,v|
    s.puts "#{k}: #{v}\r\n"
  end
  s.puts "\r\n"
  if body != ""
    s.puts body
  end
 
  status = s.gets.split(' ')[1]
  if status.start_with?('2')
    puts "OK".colorize(:green)
  else
    puts "FAIL (#{status})".colorize(:red)
    $error = true
  end

  s.close
end
  

$error = false
YAML.load(File.open(ARGV[0])).each do |r|
  request(r['method'], r['path'], r['headers'], r['body'])
end

exit ($error)? 1 : 0

