class Integration < ApplicationRecord
  belongs_to :app
  
  SUPPORTED_SERVICES = %w(outgoing_webhook ifttt slack datadog)
  INTEGRATION_TOKEN_LEN = 40
  INTEGRATION_TOKEN_PREFIX_LEN = 20
  
  validates :token_prefix, uniqueness: true
  validates :service, inclusion: { in: SUPPORTED_SERVICES },
      uniqueness: { scope: :app_id }
  validate :config_is_json
  validate :token_prefix_is_prefix
  
  # TODO: validate contents in the config JSON
  
  def config_is_json
    JSON.parse(self.config)
  rescue
    errors.add(:config, "is not valid JSON.")
  end

  def token_prefix_is_prefix
    if self.token && !self.token.start_with?(self.token_prefix)
      errors.add(:token_prefix, "is not prefix of integration (BUG).")
    end
  end

  def reset_token
    token = RandomIdGenerator::generate(INTEGRATION_TOKEN_LEN)
    self.token_prefix = token[0, INTEGRATION_TOKEN_PREFIX_LEN]
    self.token = token
  end
  
  def self.lookup_by_token(token)
    prefix = token[0, INTEGRATION_TOKEN_PREFIX_LEN]
    integration = Integration.find_by_token_prefix(prefix)
    
    if integration && ActiveSupport::SecurityUtils.secure_compare(integration.token, token)
      integration
    else
      nil
    end
  end  
end
