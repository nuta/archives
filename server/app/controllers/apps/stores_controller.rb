class Apps::StoresController < Apps::ApplicationController
  before_action :set_app
  before_action :set_app_store, only: [:show, :update, :destroy]

  def index
    @app_stores = @app.app_stores.all
  end

  def show
  end

  def create
    @app_store = @app.app_stores.new(app_store_params)

    if @app_store.save
      render :show, status: :created, location: app_store_url(@app.name, @app_store.key)
    else
      render json: @app_store.errors, status: :unprocessable_entity
    end
  end

  def update
    if @app_store.update(app_store_params)
      render :show, status: :ok, location: app_store_url(@app.name, @app_store.key)
    else
      render json: @app_store.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @app_store.destroy
  end

  private

  def set_app_store
    @app_store = @app.app_stores.find_by_key(params[:key])
  end

  def app_store_params
    params.require(:store).permit(:key, :value, :data_type)
  end
end
