class Devices::ConfigsController < Devices::ApplicationController
  before_action :set_device
  before_action :set_config, only: [:show, :update, :destroy]

  def index
    @configs = @device.configs.all
  end

  def show
  end

  def create
    @config = @device.configs.new(config_params)
    @config.save!
    render :show, status: :created, location: device_config_url(@device.name, @config.key)
  end

  def update
    @config.update!(config_params)
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
