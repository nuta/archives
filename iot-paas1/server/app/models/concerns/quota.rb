module Quota
  extend ActiveSupport::Concern

  module ClassMethods
    def quota(scope:, limit:)
      unless method_defined?(:validate_quota)
        include InstanceMethods

        validate :validate_quota
        class_attribute :quotas

        self.quotas = []
      end

      self.quotas << { scope: scope, limit: limit }
    end
  end

  module InstanceMethods
    def validate_quota
      quotas.each do |quota|
        scope = quota[:scope]
        limit = quota[:limit]

        if self.class.where(scope => self.try(scope)).count >= limit
          errors.add(:base, 'reached the quota limit')
        end
      end
    end
  end
end
