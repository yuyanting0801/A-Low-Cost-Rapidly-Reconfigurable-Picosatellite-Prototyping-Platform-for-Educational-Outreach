/**
 * 3D acceleration visualisation (Three.js): scene, camera and renderer setup,
 * plus the arrow update interface.
 */
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { CSS2DRenderer, CSS2DObject } from 'three/addons/renderers/CSS2DRenderer.js';

let arrowHelper, scene, camera, renderer, labelRenderer, controls;

export function initThree() {
    const container = document.getElementById('accel3d');
    const width = container.clientWidth, height = 350;

    scene = new THREE.Scene();
    scene.background = new THREE.Color(0xfefcf8);

    camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 1000);
    camera.position.set(3, 2, 5);

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(width, height);
    container.appendChild(renderer.domElement);

    labelRenderer = new CSS2DRenderer();
    labelRenderer.setSize(width, height);
    labelRenderer.domElement.style.position = 'absolute';
    labelRenderer.domElement.style.top = '0px';
    labelRenderer.domElement.style.left = '0px';
    labelRenderer.domElement.style.pointerEvents = 'none';
    container.appendChild(labelRenderer.domElement);

    controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;

    scene.add(new THREE.AxesHelper(1.6));

    const sphereGeo = new THREE.SphereGeometry(1.5, 32, 32);
    const sphereMat = new THREE.MeshBasicMaterial({ color: 0xccccaa, wireframe: true, transparent: true, opacity: 0.2 });
    scene.add(new THREE.Mesh(sphereGeo, sphereMat));

    const gridHelper = new THREE.GridHelper(4, 20, 0xb0aa90, 0xd0cab8);
    gridHelper.position.y = -1.5;
    scene.add(gridHelper);

    arrowHelper = new THREE.ArrowHelper(new THREE.Vector3(1, 0, 0), new THREE.Vector3(0, 0, 0), 1, 0x8f9b7e, 0.3, 0.2);
    scene.add(arrowHelper);

    function makeLabel(text, color, pos) {
        const div = document.createElement('div');
        div.textContent = text;
        div.style.color = color;
        div.style.background = 'rgba(255,255,240,0.7)';
        div.style.padding = '2px 6px';
        div.style.borderRadius = '12px';
        div.style.border = `1px solid ${color}`;
        const label = new CSS2DObject(div);
        label.position.copy(pos);
        return label;
    }
    scene.add(makeLabel('X', '#ff6666', new THREE.Vector3(1.8, 0, 0)));
    scene.add(makeLabel('Y', '#66ff66', new THREE.Vector3(0, 1.8, 0)));
    scene.add(makeLabel('Z', '#6666ff', new THREE.Vector3(0, 0, 1.8)));

    const ambientLight = new THREE.AmbientLight(0x404040);
    scene.add(ambientLight);
    const dirLight = new THREE.DirectionalLight(0xffffff, 0.8);
    dirLight.position.set(1, 2, 1);
    scene.add(dirLight);

    window.addEventListener('resize', () => {
        const w = container.clientWidth;
        camera.aspect = w / height;
        camera.updateProjectionMatrix();
        renderer.setSize(w, height);
        labelRenderer.setSize(w, height);
    });

    animate();
}

function animate() {
    requestAnimationFrame(animate);
    controls.update();
    renderer.render(scene, camera);
    labelRenderer.render(scene, camera);
}

export function updateArrow(x, y, z) {
    if (!arrowHelper) return;
    let mag = Math.sqrt(x * x + y * y + z * z);
    if (mag < 0.01) mag = 0.01;
    const dir = new THREE.Vector3(x, y, z).normalize();
    let len = Math.min(mag / 2.0, 1.2);
    if (len < 0.2) len = 0.2;
    arrowHelper.setDirection(dir);
    arrowHelper.setLength(len, 0.3, 0.2);
    const intensity = Math.min(mag / 3.0, 1.0);
    arrowHelper.setColor(new THREE.Color().setHSL(0.10, 0.8, 0.55 - intensity * 0.2));
}
