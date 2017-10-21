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
          data_type = determine_data_type(value)
          store = device.device_stores.where(key: key).first_or_initialize
          store.value = value.to_s
          store.data_type = data_type
          store.save!
        end
      end
    end

  rescue => e
    logger.warn "failed to call webhook: #{e}"
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
