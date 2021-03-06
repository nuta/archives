class Devices::ConfigsController < Devices::ApplicationController
  before_action :set_device
  before_action :set_config, only: [:show, :destroy]

  def index
    @configs = @device.configs.all
  end

  def show
  end

  def update
    @config = @device.configs.find_or_initialize_by(key: params[:key])
    @config.update(config_params)
    @config.save!
    render :show, status: :ok, location: device_config_url(@device.name, @config.key)
  end

  def destroy
    @config.destroy
  end

  private

  def set_config
    @config = @device.configs.find_by_key(params[:key])
  end

  def config_params
    params.require(:config).permit(:key, :value, :data_type)
  end
end
