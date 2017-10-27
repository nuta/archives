class Apps::ApplicationController < ApplicationController
  def set_app
    @app = @user.apps.find_by_name!(params[:app_name])
  end
end
