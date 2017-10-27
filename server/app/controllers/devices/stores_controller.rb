class Devices::StoresController < Devices::ApplicationController
  before_action :set_device_store, only: [:show, :update, :destroy]

  # GET /device_stores
  # GET /device_stores.json
  def index
    @device_stores = DeviceStore.all
  end

  # GET /device_stores/1
  # GET /device_stores/1.json
  def show
  end

  # POST /device_stores
  # POST /device_stores.json
  def create
    @device_store = DeviceStore.new(device_store_params)

    if @device_store.save
      render :show, status: :created, location: @device_store
    else
      render json: @device_store.errors, status: :unprocessable_entity
    end
  end

  # PATCH/PUT /device_stores/1
  # PATCH/PUT /device_stores/1.json
  def update
    if @device_store.update(device_store_params)
      render :show, status: :ok, location: @device_store
    else
      render json: @device_store.errors, status: :unprocessable_entity
    end
  end

  # DELETE /device_stores/1
  # DELETE /device_stores/1.json
  def destroy
    @device_store.destroy
  end

  private
    # Use callbacks to share common setup or constraints between actions.
    def set_device_store
      @device_store = DeviceStore.find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def device_store_params
      params.require(:device_store).permit(:name, :value, :device_id)
    end
end
