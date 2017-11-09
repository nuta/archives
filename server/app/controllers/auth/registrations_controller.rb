class Auth::RegistrationsController < DeviseTokenAuth::RegistrationsController
  prepend_before_action :verify_tos_agreement, only: [:create]
#  prepend_before_action :verify_recaptcha, only: [:create]

  private

  def verify_tos_agreement
    if params[:agree_tos] != 'yes'
      render json: { errors: ["agree_tos is not `yes'."] }, status: :unprocessable_entity, layout: false
    end
  end

  def verify_recaptcha
    resp = RestClient.post 'https://www.google.com/recaptcha/api/siteverify', {
      secret: Rails.application.secrets.recaptcha_secret,
      response: params[:recaptcha],
      remoteip: request.remote_ip
    }

    result = JSON.parse(resp.body)
    if resp.code != 200 || !result['success']
      logger.warn "reCAPTCHA failure: #{result['error-codes']}"
      render json: { errors: ['reCAPTCHA failure'] }, status: :unprocessable_entity, layout: false
    end
  end
end
