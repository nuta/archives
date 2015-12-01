module LoginHelper
  NAME = 'guest'
  PASSWORD = 'password'

  def login
    basic = ActionController::HttpAuthentication::Basic
    name   = NAME
    passwd = PASSWORD
    request.env['HTTP_AUTHORIZATION'] = basic.encode_credentials(name, passwd)
  end

  def password
    PASSWORD
  end

  def login_name
    NAME
  end
end
