module MakeStack
  def self.settings
    @settings ||= load_config('settings')
  end

  def self.os_releases
    @os_releases ||= load_config('os_releases')
  end

  private

  def self.load_config(name)
    suffix = (Rails.env.production?) ? '.production' : ".development"
    filename = "#{name}#{suffix}.yml"
    YAML.load(open("#{Rails.root}/config/#{filename}").read).with_indifferent_access
  end
end
