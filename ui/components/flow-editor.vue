<template>
  <div class="flow-editor" @mousedown="mousedownOnWall" @mousemove="mousemoveOnWall" @mouseup="mouseupOnWall">
    <div class="nodes-box">
      <div class="node" v-for="(node, type) in defs" :key="node.name"
       data-component="newNode" :data-node-type="type">
        <div class="icon">
          <i class="fa" :class="[node.icon]"></i>
        </div>
        <div class="content">
          <div class="name">{{ node.name }}</div>
        </div>
      </div>
    </div>

    <div class="wall-container">
      <div class="wall" ref="wall">
        <svg class="svg-canvas">
          <g class="links" ref="links">
            <template v-for="(node, id) in nodes">
              <template v-for="(dests, srcPort) in node.outputs">
                <path v-for="dest in dests" class="link"
                 :d="computeLinkCurve(id, srcPort, dest.node, dest.port)"></path>
              </template>
            </template>
          </g>
        </svg>

        <div class="dragging-node node" ref="draggingNode" v-show="draggingNewNode">
          <div class="icon">
            <i class="fa" :class="[draggingNode.icon]"></i>
          </div>
          <div class="content">
            <div class="name">{{ draggingNode.name }}</div>
          </div>
        </div>

        <div class="nodes">
          <div class="node" v-for="(node, id) in nodes" :key="id"
           @dblclick="openPopup(id)"
           data-component="node" :data-node-id="id" :data-node-type="node.type"
           :style="{
             top: '' + node.position.y + 'px',
             left: '' + node.position.x + 'px'
           }">

            <div class="icon">
              <i class="fa" :class="[defs[node.type].icon]"></i>
            </div>
            <div class="content">
              <div class="name">{{ node.name }}</div>
            </div>

            <div data-component="port" class="input-port"
            data-node-port="0" data-node-port-type="input"
             v-if="defs[node.type].category === 'output' || defs[node.type].category === 'function'"></div>
            <div data-component="port" class="output-port"
             data-node-port="0" data-node-port-type="output"
             v-if="defs[node.type].category === 'input' || defs[node.type].category === 'function'"></div>

             <div class="popup-editor" v-if="node.editing" data-component="popup">
               <div class="triangle"></div>
               <div class="content">
                 <form @submit.prevent="saveNode(id)" :id="'popup-' + id">
                   <label>Name</label>
                   <input type="text" placeholder="Label" :value="node.name">

                  <template v-for="option in defs[node.type].options">
                    <label>{{ option.name }}</label>
                    <input type="text" v-if="option.type == 'text'" :value="node.options[option.name]"
                     :data-form-key="option.name">
                    <input type="number" v-if="option.type == 'integer'" :value="node.options[option.name]"
                     :data-form-key="option.name">
                    <select :data-form-key="option.name" v-if="option.type == 'select'" :value="node.options[option.name]">
                      <option v-for="opt in option.options" :value="opt">
                      {{ opt }}
                      </option>
                    </select>
                  </template>

                   <input type="submit" value="save" class="primary">
                   <button @click.prevent="closePopup(id)">close</button>
                 </form>
               </div>
             </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  props: ['nodes', 'defs'],
  data() {
    return {
      draggingTarget: 'pointer',
      draggingNewNode: false,
      mouseTarget: null,
      draggingNode: {}
    }
  },
  methods: {
    openPopup(nodeId) {
      if (this.editingNode) {
        this.nodes[this.editingNode].editing = false
      }

      this.editingNode = nodeId
      this.nodes[nodeId].editing = true
    },
    closePopup(nodeId) {
      this.editingNode = null
      this.nodes[nodeId].editing = false
    },
    saveNode(nodeId) {
      const elems = document.querySelectorAll(`#popup-${nodeId} [data-form-key]`)
      for (const elem of elems) {
        const name = elem.getAttribute('data-form-key')
        const value = elem.value
        this.nodes[nodeId].options[name] = value
      }

      this.closePopup(nodeId)
    },
    computeLinkCurve(srcNodeId, srcPort, destNodeId, destPort) {
      const srcPortElem = document.querySelector(
        `[data-node-id="${srcNodeId}"] [data-node-port="${srcPort}"][data-node-port-type="output"]`
      )

      if (!srcPortElem || !this.$refs.wall) {
        // The element is not ready. Don't draw the link because
        // we don't know the size of the node element.
        return ''
      }

      const destPortElem = document.querySelector(
        `[data-node-id="${destNodeId}"] [data-node-port="${destPort}"][data-node-port-type="input"]`
      )

      const wallRect = this.$refs.wall.getBoundingClientRect()
      const srcPortRect = srcPortElem.getBoundingClientRect()
      const destPortRect = destPortElem.getBoundingClientRect()
      const portWidth = srcPortRect.width
      const fromX = srcPortRect.x - wallRect.x + (portWidth / 2)
      const fromY = srcPortRect.y - wallRect.y + (portWidth / 2)
      const toX   = destPortRect.x - wallRect.x + (portWidth / 2)
      const toY   = destPortRect.y - wallRect.y + (portWidth / 2)
      const offsetFromPort = 30
      const sharpness = 35

      // TODO: Draw a beautiful bezier curve.
      return `
        M ${fromX}, ${fromY}
        C ${fromX + offsetFromPort + sharpness}, ${fromY}
          ${toX - sharpness}, ${toY}
          ${toX} ${toY}
      `
    },
    mousedownOnWall(event) {
      let component = ''
      let target = event.target
      while (target) {
        component = target.getAttribute('data-component')
        if (component) {
          break
        }

        target = target.parentElement
      }

      if (component === 'popup') {
        // Ignore clicking onto a popup node editor.
        return
      }

      if (this.editingNode) {
        this.nodes[this.editingNode].editing = false
      }

      switch (component) {
        case 'newNode':
          this.draggingTarget = 'newNode'
          this.draggingNodeType = target.getAttribute('data-node-type')
          this.draggingNode = this.defs[this.draggingNodeType]
          this.mouseTarget = target
          this.$refs.draggingNode.style.top = '' + event.clientY + 'px'
          this.$refs.draggingNode.style.left = '' + event.clientX + 'px'
          this.draggingNewNode = true
          break;
        case 'node':
          const targetRect = target.getBoundingClientRect()
          this.draggingTarget = 'node'
          this.mouseTarget = target
          this.originalX = targetRect.x
          this.originalY = targetRect.y
          this.originalEventX = event.clientX
          this.originalEventY = event.clientY
          break;
        case 'port':
          this.draggingTarget = 'link'
          this.srcPortElem = target
          this.srcNode = target.parentElement.getAttribute('data-node-id')
          this.srcPort = parseInt(target.getAttribute('data-node-port'))
          this.srcPortType = target.getAttribute('data-node-port-type')

          this.linkPath = document.createElementNS("http://www.w3.org/2000/svg", "path")
          this.linkPath.classList.add('link')
          const portRect = target.getBoundingClientRect()
          const wallRect = this.$refs.wall.getBoundingClientRect()
          const srcX  = Math.floor(portRect.x - wallRect.x + 4)
          const srcY  = Math.floor(portRect.y - wallRect.y + 4)
          const destX = Math.floor(event.clientX - wallRect.x)
          const destY = Math.floor(event.clientY - wallRect.y)
          this.linkPath.setAttribute('d', `M ${srcX} ${srcY} L ${destX} ${destY}`)
          this.$refs.links.appendChild(this.linkPath)
          break;
      }
    },
    mousemoveOnWall(event) {
      if (this.draggingTarget === 'cursor')
        return

      switch (this.draggingTarget) {
        case 'newNode': {
          const rect = this.$refs.draggingNode.getBoundingClientRect()
          const x = event.clientX - rect.width / 2
          const y = event.clientY - rect.height / 2
          this.$refs.draggingNode.style.top = '' + y + 'px'
          this.$refs.draggingNode.style.left = '' + x + 'px'
          this.draggingNewNode = true
          break;
        }
        case 'node': {
          const id = this.mouseTarget.getAttribute('data-node-id')
          const rect = this.$refs.wall.getBoundingClientRect()
          const x = this.originalX + (event.clientX - this.originalEventX - rect.x)
          const y = this.originalY + (event.clientY - this.originalEventY - rect.y)
          this.nodes[id].position.x = Math.round(x / 10) * 10
          this.nodes[id].position.y = Math.round(y / 10) * 10
          break
        }
        case 'link': {
          const portRect = this.srcPortElem.getBoundingClientRect()
          const wallRect = this.$refs.wall.getBoundingClientRect()
          const srcX  = Math.floor(portRect.x - wallRect.x + 8)
          const srcY  = Math.floor(portRect.y - wallRect.y + 8)
          const destX = Math.floor(event.clientX - wallRect.x)
          const destY = Math.floor(event.clientY - wallRect.y)
          this.linkPath.setAttribute('d', `M ${srcX} ${srcY} L ${destX} ${destY}`)
          break
        }
      }
    },
    mouseupOnWall(event) {
      switch (this.draggingTarget) {
        case 'newNode':
          this.draggingNewNode = false
          this.$refs.draggingNode.style.top = '' + event.clientX + 'px'
          this.$refs.draggingNode.style.left = '' + event.clientY + 'px'
          const rect = this.$refs.wall.getBoundingClientRect()
          const x = event.clientX - rect.x
          const y = event.clientY - rect.y

          const id = 'v_' + Math.floor(Math.random() * 100000000).toString(16)
          this.nodes[id] ={
            type:  this.draggingNodeType,
            name: this.draggingNode.name,
            position: { x, y },
            outputs: [],
            options: {}
          }
          break;
        case 'link':
          this.linkPath.parentElement.removeChild(this.linkPath)
          let target = event.target
          let destNode = null
          let destPort = null
          while (target) {
            if (destNode === null) {
              destNode = target.getAttribute('data-node-id')
            }

            if (destPort === null) {
              destPort = parseInt(target.getAttribute('data-node-port'))
            }

            if (destNode && destPort) {
              break
            }

            target = target.parentElement
          }

          if (destNode) {
            const destPortType = event.target.getAttribute('data-node-port-type')
            if (destPortType === this.srcPortType) {
              break
            }

            switch (this.srcPortType) {
              case 'output':
                if (!this.nodes[this.srcNode].outputs[this.srcPort])
                  this.nodes[this.srcNode].outputs[this.srcPort] = []

                this.nodes[this.srcNode].outputs[this.srcPort].push({
                  node: destNode,
                  port: destPort
                })
                break
              case 'input':
                if (!this.nodes[destNode].outputs[destPort])
                  this.nodes[destNode].outputs[destPort] = []

                this.nodes[destNode].outputs[destPort].push({
                  node: this.srcNode,
                  port: this.srcPort
                })
                break
            }
          }
          break;
      }

      this.draggingTarget = 'cursor'

      // Don't update if this.editingNode is true in order to
      // preserve form values in the popup.
      if (!this.editingNode) {
        this.$forceUpdate()
      }

      this.$emit('changed', JSON.stringify(this.nodes))
    }
  },
  mounted() {
    // Recompute link positions.
    this.$forceUpdate()
  }
}
</script>


<style lang="scss">
path.link {
  z-index: 1001;
  stroke: #666;
  fill: none;
  stroke-width: 4px;
}
</style>

<style lang="scss" scoped>
.flow-editor {
  display: flex;
  justify-content: space-between;
  flex: 1;

  .nodes-box {
    padding: 30px 20px;
    overflow-y: scroll;
    background: var(--bg2-color);
  }

  .wall-container {
    position: relative;
    overflow: scroll;
    flex: 1;

    .wall {
      background: var(--bg1-color);
      min-width: 1000px;
      min-height: 1000px;
    }
  }
}

.svg-canvas {
  width: 1000px;
  height: 1000px;
}

.links {
  z-index: 1000;
}

.nodes-box {
  .node {
    margin-bottom: 20px;
  }
}

.nodes {
  .node {
    position: absolute;
  }
}

.dragging-node {
  opacity: 0.7;
  position: fixed;
}

$node-height: 30px;
.node {
  background: #fefefe;
  border: 3px solid #cacaca;
  border-radius: 3px;
  box-sizing: border-box;
  height: $node-height;
  width: fit-content;
  user-select: none;
  display: flex;
  flex: 0 0 auto;

  &:hover {
    cursor: move;
  }

  & > .icon {
    padding: 3px 5px;
  }

  & > .content {
    background: #dadada;
    flex: 1;
    padding: 2px 3px;
  }

  $port-size: 8px;
  .input-port, .output-port {
    background: #fefefe;
    position: absolute;
    border: 3px solid #555;
    width: $port-size;
    height: $port-size;

    &:hover {
      cursor: crosshair;
    }
  }

  .input-port {
    left: 0;
    top: 0;
    transform: translate(-10px, 6px);
  }

  .output-port {
    right: 0;
    top: 0;
    transform: translate(10px, 6px);
  }
}

.popup-editor {
  z-index: 1000;
  position: absolute;
  left: 0;
  top: 40px;
  background: #ffffff;
  box-sizing: border-box;
  border: 1px solid #dadada;
  border-radius: 5px;
  min-width: 300px;
  min-height: 250px;

  .triangle {
    width: 0;
    height: 0;
    position: relative;
    top: -10px;
    left: 5px;
    border-style: solid;
    border-width: 0 7.5px 10px 7.5px;
    border-color: transparent transparent #dadada transparent;
  }

  .content {
    padding: 15px;
  }
}
</style>
