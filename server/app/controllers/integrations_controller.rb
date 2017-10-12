class IntegrationsController < ApplicationController
  before_action :set_app
  before_action :set_integration, only: [:show, :update, :destroy]

  def index
    @integrations = @app.integrations.all
  end

  def show
  end

  def create
    @integration = Integration.new(integration_params)
    @integration.app = @app
    @integration.reset_token

    if @integration.save
      render :show, status: :created, location: @integration
    else
      render json: @integration.errors, status: :unprocessable_entity
    end
  end

  def update
    if @integration.update(integration_params)
      render :show, status: :ok, location: @integration
    else
      render json: @integration.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @integration.destroy
  end

  private

  def set_integration
    @integration = @app.integrations.find_by_service(params[:service])
  end

  def integration_params
    params.require(:integration).permit(:service, :config, :comment)
  end
end
