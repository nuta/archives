class SourceFilesController < ApplicationController
  before_action :set_app
  before_action :set_source_file, only: [:show, :update, :destroy]

  def index
    @source_files = @app.source_files.all
  end

  def show
  end

  def update
    @source_file = @app.source_files.find_or_initialize_by(path: params[:path])

    if @source_file.update(source_file_params)
      render :show, status: :ok
    else
      render json: @source_file.errors, status: :unprocessable_entity
    end
  end

  def destroy
    @source_file.destroy
  end

  private

  def set_source_file
    @source_file = @app.source_files.find_by_path(params[:path])
  end

  def source_file_params
    params.require(:source_file).permit(:path, :body)
  end
end
