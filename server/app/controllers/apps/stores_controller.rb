class Apps::StoresController < Apps::ApplicationController
  before_action :set_app
  before_action :set_store, only: [:show, :update, :destroy]

  def index
    @stores = @app.stores.all
  end

  def show
  end

  def create
    @store = @app.stores.new(store_params)

    if @store.save
      render :show, status: :created, location: app_store_url(@app.name, @store.key)
    else
      render json: @store.errors, status: :unprocessable_entity
    end
  end

  def update
    if @store.update(store_params)
      render :show, status: :ok, location: app_store_url(@app.name, @store.key)
    else
      render json: @store.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @store.destroy
  end

  private

  def set_store
    @store = @app.stores.find_by_key(params[:key])
  end

  def store_params
    params.require(:store).permit(:key, :value, :data_type)
  end
end
