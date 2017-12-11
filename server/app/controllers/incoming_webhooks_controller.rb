class IncomingWebhooksController < ApplicationController
  skip_before_action :authenticate
  before_action :set_integration
  before_action :set_app
  before_action :set_user

  def invoke
    case params[:command]
    when 'update_app_configs'
      update_configs('App', @app.id, params[:configs].permit!.to_h)
    when 'update_device_configs'
      device = @user.devices.find_by_name!(params[:device])
      update_configs('Device', device.id, params[:configs].permit!.to_h)
    else
      raise ActionController::BadRequest.new(), "unknown command"
    end

    head :ok
  end

  private

  def set_integration
    @integration = Integration.lookup_by_token(params[:token])
    unless @integration
      head :forbidden
      return
    end
  end

  def set_app
    @app = @integration.app
  end

  def set_user
    @user ||= @app.user
  end

  def update_configs(owner_type, owner_id, configs)
    unless configs.is_a?(Hash)
      raise ActionController::BadRequest.new(), "`configs' must be a object'"
    end

    ActiveRecord::Base.transaction do
      configs.each do |key, value|
        config = Config.where(owner_type: owner_type, owner_id: owner_id, key: key).first_or_create
        config.data_type = determine_data_type(value)
        config.value = value
        config.save!
      end
    end
  end

  private

  def determine_data_type(value)
    case value
    when String then 'string'
    when TrueClass, FalseClass then 'bool'
    when Float then 'float'
    when Integer then 'integer'
    else raise 'unknown data type'
    end
  end
end
