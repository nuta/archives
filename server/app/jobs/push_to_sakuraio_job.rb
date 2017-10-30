class PushToSakuraioJob < ApplicationJob
  queue_as :default
  throttle limit: MakeStack.settings[:push_to_sakuraio_limit_per_hour], period: 1.hour

  def execute(webhook_token:, module_id:, payload:)
    # "\xAA\xBB\xCC\xDD..." => [["AABBCCDD..."], [], ...]
    arrays = payload.scan(/./).map {|b| b.ord.to_s(16).rjust(2, '0') }.each_slice(8).map(&:join).to_a

    # [..., ["AABBCCDD"]] => [..., ["AABBCCDD00000000"]]
    arrays[-1] = arrays[-1].ljust(8 * 2, '0')

    channels = arrays.map.with_index do |ch, index|
      {
        channel: index,
        type: 'b',
        value: ch
      }
    end

    channels << { channel: 16, type: 'i', value: 0x55aa } # COMMIT

    body = {
      type: 'channels',
      module: module_id,
      payload: {
        channels: channels
      }
    }

    url = "https://api.sakura.io/incoming/v1/#{webhook_token}"
      RestClient.post(url, body.to_json, content_type: :json)
  end
end
