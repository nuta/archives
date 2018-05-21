class Apps::LogController < Apps::ApplicationController
  before_action :set_app

  def index
    @log = @app.get_log(params[:since])
  end
end
