require 'rails_helper'

RSpec.describe PushToSakuraioJob, type: :job do

  context "sakura.io lives" do
    it 'sends a payload properly' do
      stub_request(:post, "https://api.sakura.io/incoming/v1/a-webhook-token")
        .with(body: "{\"type\":\"channels\",\"module\":\"xXyYzZ789\",\"payload\":{\"channels\":[{\"channel\":0,\"type\":\"b\",\"value\":\"6162636465666768\"},{\"channel\":1,\"type\":\"b\",\"value\":\"696a6b6c6d6e0000\"},{\"channel\":16,\"type\":\"i\",\"value\":21930}]}}")
        .to_return(status: 200)

      expect {
        PushToSakuraioJob.perform_now(
          webhook_token: 'a-webhook-token',
          module_id: 'xXyYzZ789',
          payload: 'abcdefghijklmn'
        )
      }.not_to raise_exception
    end
  end

  context "sakura.io is down" do
    it 'raises an exception' do
      stub_request(:post, "https://api.sakura.io/incoming/v1/a-webhook-token")
        .with(body: "{\"type\":\"channels\",\"module\":\"xXyYzZ789\",\"payload\":{\"channels\":[{\"channel\":0,\"type\":\"b\",\"value\":\"6162636465666768\"},{\"channel\":1,\"type\":\"b\",\"value\":\"696a6b6c6d6e0000\"},{\"channel\":16,\"type\":\"i\",\"value\":21930}]}}")
        .to_timeout

      expect {
        PushToSakuraioJob.perform_now(
          webhook_token: 'a-webhook-token',
          module_id: 'xXyYzZ789',
          payload: 'abcdefghijklmn'
        )
      }.to raise_exception(RestClient::Exceptions::OpenTimeout)
    end
  end
end
