class PluginsController < ApplicationController
  skip_before_action :authenticate

  def download
    github_repo = (official?) ? 'seiyanuta/makestack' : "#{params[:org]}/#{params[:name]}"
    name = params[:name]
    if Rails.env.production?
      # TODO
      head :not_implemented
    else
      if name == 'app-runtime'
        # Build app-runtime in the local repository.
        plugin_zip_path = build_local_app_runtime
        send_file plugin_zip_path, content_type: 'application/zip'
      end
    end
  end

  private

  def official?
    (params[:org] == '_')
  end

  unless Rails.env.production?
    def build_local_app_runtime
      plugin_zip_path = File.join(Dir.home, '.makestack', 'server', 'app-runtime.plugin.zip')
      FileUtils.mkdir_p(File.dirname(plugin_zip_path))
      FileUtils.rm_f(plugin_zip_path)

      makestack_command = File.join(File.dirname(__FILE__), '../../../cli/bin/makestack')
      app_runtime_dir = File.join(File.dirname(__FILE__), '../../../libs/app-runtime')
      system("#{makestack_command} build-plugin #{app_runtime_dir} #{plugin_zip_path}")
      plugin_zip_path
    end
  end
end
