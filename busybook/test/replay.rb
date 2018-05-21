#!/usr/bin/env ruby
require 'yaml'
require 'socket'
require 'colorize'

HOST = 'localhost'
PORT = 3000

def request(method, path, headers, body, expected_status, expected_response)
  print "#{method} #{path}   "
  STDOUT.flush

  s = TCPSocket.open HOST, PORT
  s.puts "#{method} #{path} HTTP/1.1\r\n"
  s.puts "Connection: close\r\n"
  s.puts "Content-Length: #{body.bytesize}\r\n"

  headers.each do |k,v|
    s.puts "#{k}: #{v}\r\n"
  end

  s.puts "\r\n"
  if body != ""
    s.puts body
  end

  status = s.gets.split(' ')[1].to_i
  if status != expected_status
    puts "FAIL (expected #{expected_status}, but #{status})".colorize(:red)
    $error = true
    return
  end

  response = ''
  if %w(GET).include?(method)
    length_line = false
    while line = s.gets
      # supports 'Transfer-Encoding: chunked' only
      break if line == "0\r\n"
      response += line
    end
  end

  s.close

  if not expected_response or response.force_encoding("UTF-8").include?(expected_response)
    puts "OK".colorize(:green)
    return true
  else
    puts "FAIL (expected '#{expected_response}' in the response)".colorize(:red)
    return false
  end
end

results = YAML.load(File.open(ARGV[0])).map do |r|
  request(r['method'], r['path'], r['headers'], r['body'], r['status'], r['includes'])
end

exit (results.all?)? 0 : 1

