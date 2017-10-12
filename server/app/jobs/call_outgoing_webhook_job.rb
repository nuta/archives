class CallOutgoingWebhookJob < ApplicationJob
  queue_as :default

  def perform(url:, params: {}, body:, accept_stores: false, device: nil)
    resp = RestClient.post(url, body.to_json, params: params, content_type: :json)

    # Update device stores.
    if accept_stores
      ActiveRecord::Base.transaction do
        JSON.parse(resp.body).fetch("stores", {}).each do |key, value|
          store = device.device_stores.where(key: key).first_or_create
          store.value = value
          store.save!
        end
      end
    end

  rescue => e
    logger.warn "failed to call webhook: #{e}"
  end
end
