FactoryBot.define do
  factory :source_file do
    app
    sequence(:path) {|n| "file#{n}.js" }
    body { FFaker::Lorem.sentence }
  end
end
