class Devices::StoresController < Devices::ApplicationController
  before_action :set_device
  before_action :set_store, only: [:show, :update, :destroy]

  def index
    @stores = @device.stores.all
  end

  def show
  end

  def create
    @store = @device.stores.new(store_params)

    if @store.save
      render :show, status: :created, location: device_store_url(@device.name, @store.key)
    else
      render json: @store.errors, status: :unprocessable_entity
    end
  end

  def update
    if @store.update(store_params)
      render :show, status: :ok, location: device_store_url(@device.name, @store.key)
    else
      render json: @store.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @store.destroy
  end

  private

  def set_store
    @store = @device.stores.find_by_key(params[:key])
  end

  def store_params
    params.require(:store).permit(:key, :value, :data_type)
  end
end
