module ControllerHelpers
  def login(user)
    request.headers["Accept"] = "application/json"
    request.headers.merge!(user.create_new_auth_token)
  end
end