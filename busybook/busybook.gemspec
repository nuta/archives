# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'busybook/version'

Gem::Specification.new do |spec|
  spec.name          = "busybook"
  spec.version       = Busybook::VERSION
  spec.authors       = ["Seiya Nuta"]
  spec.email         = ["nuta@seiya.me"]

  spec.summary       = %q{A CalDAV server out of the box}
  spec.description   = %q{}
  spec.homepage      = "https://github.com/nuta/busybook"

  spec.files         = `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  spec.bindir        = "exe"
  spec.executables   = ["busybook"]
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.11"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "pry"

  spec.add_runtime_dependency "git"
  spec.add_runtime_dependency "bundler"
  spec.add_runtime_dependency "colorize"
  spec.add_runtime_dependency "highline"
end
