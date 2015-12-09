class Calendar < ActiveRecord::Base
  belongs_to :user

  def props
    ActiveSupport::JSON.decode(self.propxml)
  end

  def props=(v)
    self.propxml = ActiveSupport::JSON.encode(v)
  end
end
