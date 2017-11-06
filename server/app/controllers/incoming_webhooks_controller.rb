class IncomingWebhooksController < ApplicationController
  skip_before_action :authenticate
  before_action :set_integration
  before_action :set_app
  before_action :set_user

  def invoke
    case params[:command]
    when 'update_app_stores'
      update_stores('App', @app.id, params[:stores].permit!.to_h)
    when 'update_device_stores'
      device = @user.devices.find_by_name!(params[:device])
      update_stores('Device', device.id, params[:stores].permit!.to_h)
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

  def update_stores(owner_type, owner_id, stores)
    unless stores.is_a?(Hash)
      raise ActionController::BadRequest.new(), "`stores' must be a object'"
    end

    ActiveRecord::Base.transaction do
      stores.each do |key, value|
        store = Store.where(owner_type: owner_type, owner_id: owner_id, key: key).first_or_create
        store.data_type = determine_data_type(value)
        store.value = value
        store.save!
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
