class PluginsController < ApplicationController
  def download
    github_repo = (official?) ? 'seiyanuta/makestack' : "#{params[:org]}/#{params[:name]}"
    plugin = params[:name]

    if Rails.env.development? && official?
      plugin_zip_path = build_plugin_locally(plugin)
      send_file plugin_zip_path, content_type: 'application/zip'
    else
      r = RestClient.get "https://github.com/#{github_repo}/releases/latest",
        accept: 'application/json'

      latest_tag = JSON.parse(r.body)['tag_name']
      plugin_url = "https://github.com/#{github_repo}/releases/download/#{latest_tag}/#{plugin}-#{latest_tag}.plugin.zip"
      r = RestClient.get plugin_url

      render body: r.body
    end
  end

  private

  def official?
    (params[:org] == '_')
  end

  if Rails.env.development?
    def build_plugin_locally(plugin)
      plugin_zip_path = File.join(Dir.home, '.makestack', 'server', "#{plugin}.plugin.zip")
      FileUtils.mkdir_p(File.dirname(plugin_zip_path))
      FileUtils.rm_f(plugin_zip_path)

      makestack_command = File.join(File.dirname(__FILE__), '../../../client/bin/makestack')

      if plugin == 'runtime'
        plugin_dir = File.join(File.dirname(__FILE__), '../../../runtime')
      else
        plugin_dir = File.join(File.dirname(__FILE__), '../../../plugins/', plugin)
        unless Dir.exist?(plugin_dir)
          raise 'plugin not found'
        end
      end

      system("#{makestack_command} build-plugin #{plugin_dir} #{plugin_zip_path}")

      plugin_zip_path
    end
  end
end
