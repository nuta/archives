require 'net/http'

class CallOutgoingWebhookJob < ApplicationJob
  queue_as :default

  def perform(url, app_id, device_id, event, body)
    app = App.find(app_id)
    device = Device.find(device_id)
    
    response = Net::HTTP.post(URI(url), {
      app: app.name,
      device: device.name,
      event: event,
      body: body
    }.to_json)

    # Update device stores.
    ActiveRecord::Base.transaction do
      JSON.parse(response.body).fetch("stores", {}).each do |key, value|
        store = device.device_stores.where(key: key).first_or_create
        store.value = value
        store.save!
      end
    end

  rescue => e
    logger.warn "failed to call webhook: #{e}"
  end
end
