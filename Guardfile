guard :rspec, :cmd => 'bundle exec rspec' do
  watch(%r{^spec/.+_spec\.rb$})
  watch(%r{^lib/(.+)\.rb$})            {|m| "spec/lib/#{m[1]}_spec.rb" }
  watch(%r{^app/models/(.+)\.rb})      {|m| "spec/models/#{m[1]}_spec.rb" }
  watch(%r{^app/controllers/(.+)\.rb}) {|m| "spec/controllers/#{m[1]}_spec.rb" }
end

notification :off
