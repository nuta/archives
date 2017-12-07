module MakeStack
  def self.settings
    @settings ||= load_config('settings', true)
  end

  def self.os_releases
    @os_releases ||= load_config('os_releases', false)
  end

  private

  def self.load_config(name, add_env_suffix)
    if add_env_suffix
      suffix = (Rails.env.production?) ? '.production' : ".development"
    else
      suffix = ''
    end

    filename = "#{name}#{suffix}.yml"
    YAML.load(open("#{Rails.root}/config/#{filename}").read).with_indifferent_access
  end
end
