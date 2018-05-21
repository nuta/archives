module Etag
  extend ActiveSupport::Concern

  def getetag(sched)
    '"' + Digest::MD5.hexdigest(sched.ics) + '"'
  end

  def remove_etag_prefix(etag)
    if etag.start_with?("W/")
      etag[2..-1]
    else
      etag
    end
  end

end
