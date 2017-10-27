class DeploymentsController < ApplicationController
  before_action :set_app
  before_action :set_deployment, only: [:show]

  def index
    @deployments = @app.deployments.all
  end

  def show
  end

  def create
    @deployment = @app.deployments.new(deployment_params)

    if @deployment.save
      render :show, status: :created, location: app_deployment_url(@app.name, @deployment.version)
    else
      render json: @deployment.errors, status: :unprocessable_entity
    end
  end

  private

  def set_deployment
    @deployment = @app.deployments.find_by_version!(params[:version])
  end

  def deployment_params
    params.require(:deployment).permit(:image, :debug, :comment, :tag)
  end
end
