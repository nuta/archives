class HookService
  def invoke(integrations, hook, model, options)
    case model
    when Device
      case hook
      when :event_published
        handle_event_published(integrations, model, options)
      end
    end
  end

  private

  def handle_event_published(integrations, device, options)
    event = options[:event]
    body = options[:body]
 
    integrations.each do |integration|
      config = JSON.parse(integration.config)

      case integration.service
      when 'outgoing_webhook'
        CallOutgoingWebhookJob.perform_later(
          url: config['webhook_url'],
          body: { device: device.name, event: event, body: body },
          accept_stores: true,
          device: device
        )
      end
    end
  end
end