module MakeStack
  def self.settings
    @settings ||= YAML.load(open("#{Rails.root}/config/makestack.yml").read)[Rails.env].with_indifferent_access
  end
end
