<<template>
<div>
</div>
</template>

<script>
import api from "js/api"

export default {
  created() {
    api.forceLogin()
  }
}
</script>
