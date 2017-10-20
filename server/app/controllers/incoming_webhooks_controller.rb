class IncomingWebhooksController < ApplicationController
  skip_before_action :authenticate
  before_action :set_integration
  before_action :set_app
  before_action :set_user

  def invoke
    case params[:action]
    when 'update_app_stores'
      update_app_stores(@app, params[:stores])
    when 'update_devices_stores'
      device = @user.devices.find_by_name!(params[:device])
      update_device_stores(device, params[:stores])
    else
      raise ActionController::BadRequest.new(), "unknown action"
    end

    head :ok
  end

  private

  def set_integration
    @integration = Integration.lookup_by_token(params[:token])
    unless @integration
      head :not_found
      return
    end
  end

  def set_app
    @app = @integration.app
  end

  def set_user
    @user ||= @app.user
  end

  def update_app_stores(app, stores)
    unless stores.is_a(Hash)
      raise ActionController::BadRequest.new(), "`stores' must be a object'"
    end

    ActiveRecord::Base.transaction do
      stores.each do |key, value|
        store = app.stores.where(key: key).first_or_create
        store.value = value
        store.save!
      end
    end
  end

  def update_device_stores(device, stores)
    unless stores.is_a(Hash)
      raise ActionController::BadRequest.new(), "`stores' must be a object'"
    end

    ActiveRecord::Base.transaction do
      stores.each do |key, value|
        store = device.device_stores.where(key: key).first_or_create
        store.value = value
        store.save!
      end
    end
  end
end
