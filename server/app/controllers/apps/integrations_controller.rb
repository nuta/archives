class Apps::IntegrationsController < Apps::ApplicationController
  before_action :set_app
  before_action :set_integration, only: [:show, :update, :destroy]

  def index
    @integrations = @app.integrations.all
  end

  def show
  end

  def create
    @integration = @app.integrations.new(integration_params)

    if @integration.save
      render :show, status: :created, location: app_integration_url(@app.name, @integration.name)
    else
      render json: @integration.errors, status: :unprocessable_entity
    end
  end

  def update
    if @integration.update(integration_params)
      render :show, status: :ok, location: app_integration_url(@app.name, @integration.name)
    else
      render json: @integration.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @integration.destroy
  end

  private

  def set_integration
    @integration = @app.integrations.find_by_service!(params[:name])
  end

  def integration_params
    params.require(:integration).permit(:service, :config, :comment)
  end
end
