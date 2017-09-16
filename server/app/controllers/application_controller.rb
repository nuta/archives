class ApplicationController < ActionController::API
  include DeviseTokenAuth::Concerns::SetUserByToken

  before_action :configure_permitted_parameters, if: :devise_controller?
  before_action :authenticate, unless: :devise_controller?
  
  def authenticate
    authenticate_user!
    @user = current_user
  end

  def set_app
    @app = @user.apps.find_by_name!(params[:app_name])
  end

  def configure_permitted_parameters
    devise_parameter_sanitizer.permit(:account_update, keys: [])
    devise_parameter_sanitizer.permit(:sign_up, keys: [:email, :username])
  end
end
