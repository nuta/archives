class ApplicationController < ActionController::API
  include DeviseTokenAuth::Concerns::SetUserByToken

  before_action :configure_permitted_parameters, if: :devise_controller?
  before_action :authenticate, unless: :devise_controller?

  def authenticate
    authenticate_user!
    @user = current_user
  end

  def sign_and_set_authorization_header(device, payload)
    timestamp, hmac = SMMSService.sign(device, payload)
    response.headers['authorization'] = "SMMS #{timestamp} #{hmac}"
  end

  def configure_permitted_parameters
    devise_parameter_sanitizer.permit(:account_update, keys: [])
    devise_parameter_sanitizer.permit(:sign_up, keys: [:email, :username])
  end
end
