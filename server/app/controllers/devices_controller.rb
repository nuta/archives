class DevicesController < ApplicationController
  before_action :set_device, only: [:show, :log, :update, :destroy]

  def index
    @devices = @user.devices.includes(:app).all
  end

  def show
  end

  def create
    @device = @user.devices.new(device_params)

    if @device.save
      render :show, status: :created, location: @device
    else
      render json: @device.errors, status: :unprocessable_entity
    end
  end

  def update
    app = params.dig(:device, :app)
    if app
      @device.app = @user.apps.find_by_name!(app)
    end

    if @device.update(device_params)
      render :show, status: :ok, location: @device
    else
      render json: @device.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @device.destroy
  end

  private

  def set_device
    @device = @user.devices.find_by_name!(params[:name])
  end

  def device_params
    params.require(:device).permit(:name, :tag, :device_type)
  end
end
