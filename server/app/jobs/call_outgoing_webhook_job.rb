class CallOutgoingWebhookJob < ApplicationJob
  queue_as :default

  def perform(url:, params: {}, body:, accept_stores: false, device: nil)
    resp = RestClient.post(url, body.to_json, params: params, content_type: :json)

    # Update device stores.
    if accept_stores
      ActiveRecord::Base.transaction do
        stores = JSON.parse(resp.body).fetch("stores", {})
        unless stores.is_a?(Hash)
          raise "`stores' received from a webhook is not Hash"
        end
        
        stores.each do |key, value|
          # TODO: accept other data types
          store = device.device_stores.where(key: key, data_type: 'string').first_or_create
          store.value = value
          store.save!
        end
      end
    end

  rescue => e
    logger.warn "failed to call webhook: #{e}"
  end
end
