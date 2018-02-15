module MakeStack
  def self.settings
    @settings ||= load_config('settings', true)
  end

  def self.releases
    @releases ||= load_config('releases', false)
  end

  private

  def self.load_config(name, add_env_suffix)
    if add_env_suffix
      suffix = (Rails.env.production?) ? '.production' : ".development"
    else
      suffix = ''
    end

    candidates = [
      File.join(Rails.root, 'config', "#{name}#{suffix}.yml"),
      File.join(Rails.root, '..', "#{name}#{suffix}.json"),

      # `releases.json' locates at the `Rails.root' on Heroku.
      File.join(Rails.root, "#{name}#{suffix}.json")
    ]

    for filepath in candidates
      if File.exist?(filepath)
        # Note that JSON is compatible with YAML.
        return YAML.load(open(filepath).read).with_indifferent_access
      end
    end

    raise "Failed to load settings: #{name}"
  end
end
