class HeartbeatController < ApplicationController
    skip_before_action :authenticate

    def heartbeat
        version, components = parse_msp_packet
        if version != 1
            head :not_acceptable
            return
        end

        unless components.key?(:device_id)
            head :bad_request
            return
        end

        @device = Device.authenticate(components[:device_id])
        unless @device
            head :forbidden
            return
        end

        @device.append_log(components[:log])
        @device.update_attributes!(
          status: components.dig(:device_info, :state),
          last_heartbeated_at: Time.now
        )

        render body: @device.create_heartbeat_response(components[:app_version])
    end

    def os_image
      head :not_implemented
    end
  
    def app_image
      @device = Device.authenticate(params[:device_id])
      unless @device
        head :forbidden
        return
      end

      render body: @device.app_image(params[:version])
    end

    private

    # TODO: add spec for this
    def parse_msp_packet
        types = {
            0x00 => :unknown,
            0x0a => :device_id, 
            0x0b => :device_info,
            0x0c => :log,  
            0x0d => :app_version
        }

        data = request.body.read
        version, _, rest = data.unpack("CCa*")
        components = {}
        while rest.length >= 2
          header, rest = rest.unpack("Ca*")
          extended_length = header & 0x80 != 0
          type = types.fetch(header & 0x7f)
          
          if extended_length
            _, length, rest = rest.unpack("Cna*")
          else
            length, rest = rest.unpack("Ca*")
        end

          data, rest = rest.unpack("a#{length}a*")

          components[type] = case type
                             when :device_info
                                states = %w(unused new booting ready running down reboot relaunch)
                                state_id = data.ord & 0x07
                                { state: states.fetch(state_id, "unknown") }
                             when :app_version
                                data.unpack("n")[0]
                             else
                                data
                             end
        end

        return version, components
    end
end
