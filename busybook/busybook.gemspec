# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)

Gem::Specification.new do |spec|
  spec.name          = "busybook"
  spec.version       = File.read(File.expand_path('../../VERSION', __FILE__)).strip
  spec.authors       = ["Seiya Nuta"]
  spec.email         = ["nuta@seiya.me"]
  spec.summary       = %q{A CalDAV server out of the box}
  spec.description   = %q{}
  spec.homepage      = "https://github.com/seiyanuta/busybook"
  spec.license       = "Public Domain"

  spec.required_ruby_version = '>= 2.0.0'

  spec.files         = Dir['README.md', 'busybook']
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
