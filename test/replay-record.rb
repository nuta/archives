#!/usr/bin/env ruby
require 'yaml'
require 'socket'
require 'colorize'

HOST = 'localhost'
PORT = 3000

def request(method, path, headers, body, expected_status)
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
 
  status = s.gets.split(' ')[1].to_i
  if status == expected_status
    puts "OK".colorize(:green)
  else
    puts "FAIL (expected #{expected_status}, but #{status})".colorize(:red)
    $error = true
  end

  s.close
end
  

$error = false
YAML.load(File.open(ARGV[0])).each do |r|
  request(r['method'], r['path'], r['headers'], r['body'], r['status'])
end

exit ($error)? 1 : 0

