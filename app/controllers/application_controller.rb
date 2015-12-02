class ApplicationController < ActionController::Base
  skip_before_action :verify_authenticity_token

  rescue_from ActiveRecord::RecordNotFound,  with: :handle_404

  def handle_404
    head status: :not_found
  end
end
