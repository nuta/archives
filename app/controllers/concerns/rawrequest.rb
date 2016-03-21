module Rawrequest
  extend ActiveSupport::Concern

  def rawrequest
    request.body.rewind
    request.body.read.force_encoding("UTF-8")
  end                                              
end
