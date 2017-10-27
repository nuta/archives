class AppsController < ApplicationController
  before_action :set_app, only: [:show, :log, :update, :destroy]

  def index
    @apps = @user.apps.all
  end

  def show
  end

  def create
    @app = App.new(app_params)
    @app.user = @user

    if @app.save
      render :show, status: :created, location: @app
    else
      render json: @app.errors, status: :unprocessable_entity
    end
  end

  def update
    if @app.update(app_params)
      render :show, status: :ok, location: @app
    else
      render json: @app.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @app.destroy
  end

  private

  def set_app
    @app = @user.apps.find_by_name!(params[:name])
  end

  def app_params
    params.require(:app).permit(:name, :api, :os_version)
  end
end
