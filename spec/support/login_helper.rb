module LoginHelper
  LOGIN_NAME = 'guest'

  def login
    name   = LOGIN_NAME
    passwd = ''
    request.env['HTTP_AUTHORIZATION'] = ActionController::HttpAuthentication::Basic.encode_credentials(name, passwd)
  end

  def get_login_name
    LOGIN_NAME
  end
end
