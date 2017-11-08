class ApplicationController < ActionController::API
  include DeviseTokenAuth::Concerns::SetUserByToken

  before_action :configure_permitted_parameters, if: :devise_controller?
  before_action :authenticate, unless: :devise_controller?
  before_action :set_default_response_format

  def authenticate
    authenticate_user!
    @user = current_user
  end

  def set_default_response_format
    request.format = :json
  end

  def configure_permitted_parameters
    devise_parameter_sanitizer.permit(:account_update, keys: [:email])
    devise_parameter_sanitizer.permit(:sign_up, keys: [:email, :username])
  end
end
