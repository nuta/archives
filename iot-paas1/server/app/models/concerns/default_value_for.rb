module DefaultValueFor
  extend ActiveSupport::Concern

  module ClassMethods
    def default_value_for(attr, value:, &block)
      unless method_defined?(:default_value_for)
        include InstanceMethods

        before_validation :set_default_values
        class_attribute :default_values

        self.default_values = []
      end

      self.default_values << { attr: attr, value: value, block: block }
    end
  end

  module InstanceMethods
    def set_default_values
      default_values.each do |default|
        attr  = default[:attr].to_s
        block = default[:block]
        value = block ? block.call : default[:value]

        unless self.send(attr)
          self.send("#{attr}=", value)
        end
      end
    end
  end
end
