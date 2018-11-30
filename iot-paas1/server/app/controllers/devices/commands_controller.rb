class Devices::CommandsController < Devices::ApplicationController
  before_action :set_device

  def index
    render json: { results: @device.formatted_command_results }, status: :ok
  end

  def create
    command_id = @device.invoke_command!(params[:command], params[:arg])
    render json: { id: command_id }, status: :ok
  end
end
