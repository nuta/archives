  class Apps::ConfigsController < Apps::ApplicationController
  before_action :set_app
  before_action :set_config, only: [:show, :destroy]

  def index
    @configs = @app.configs.includes(:owner).all
  end

  def show
  end

  def update
    @config = @app.configs.find_or_initialize_by(key: params[:key])
    @config.update(config_params)
    @config.save!
    render :show, status: :ok, location: app_config_url(@app.name, @config.key)
  end

  def destroy
    @config.destroy
  end

  private

  def set_config
    @config = @app.configs.find_by_key(params[:key])
  end

  def config_params
    params.require(:config).permit(:key, :value, :data_type)
  end
end
