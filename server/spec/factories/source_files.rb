FactoryGirl.define do
  factory :source_file do
    app
    path { File.basename(FFaker::Filesystem.file_name) }
    body { FFaker::Lorem.sentence }
  end
end
