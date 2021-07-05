// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
 *
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 *
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.optimization;

import java.util.*;
import java.util.Map.Entry;

import org.opt4j.core.Genotype;
import org.opt4j.core.genotype.CompositeGenotype;
import org.opt4j.core.problem.Decoder;

import com.google.inject.Inject;

import net.sf.opendse.model.*;
import net.sf.opendse.model.parameter.*;
import net.sf.opendse.optimization.ImplementationWrapper;
import net.sf.opendse.optimization.ParameterDecoder;
import net.sf.opendse.optimization.ParameterMap;
import net.sf.opendse.optimization.SpecificationWrapper;

public class ModularDSEDecoder implements
    Decoder<CompositeGenotype<String, Genotype>, ImplementationWrapper>
{

    protected final Specification spec;
    protected final List<ImplementationCreatorDecoder> creatorDecoders;

    @Inject
    public ModularDSEDecoder(Set<ImplementationCreatorDecoder> creatorDecoders, ParameterDecoder parameterDecoder,
            SpecificationWrapper specWrapper) {

        this.creatorDecoders = new ArrayList<ImplementationCreatorDecoder>(creatorDecoders);
        this.parameterDecoder = parameterDecoder;
        this.spec = specWrapper.getSpecification();

        initParameters();

        // Sort creatorDecoders according to priority. Lower priorities will be earlier in the list.
        Collections.sort(this.creatorDecoders, new Comparator<ImplementationCreatorDecoder>() {
            @Override
            public int compare(ImplementationCreatorDecoder o1, ImplementationCreatorDecoder o2) {
                Integer i1 = o1.getPriority();
                Integer i2 = o2.getPriority();
                return i1.compareTo(i2);
            }
        });
    }

    @Override
    public ImplementationWrapper decode(CompositeGenotype<String, Genotype> cg) {
        ImplementationWrapper wrapper = new ImplementationWrapper(null);
        for (ImplementationCreatorDecoder creatorDecoder : creatorDecoders) {
            String name = creatorDecoder.getClass().getCanonicalName();
            System.err.println(name + " decoding [BEGIN]");
            Specification impl = creatorDecoder.decode(wrapper.getImplementation(), cg.get(name));
            if (impl != null)
                wrapper.setImplementation(impl);
            System.err.println(name + " decoding [END]");
        }
        {
            CompositeGenotype<String, Genotype> parameterGenotype = cg.get("PARAMETER");
            Specification impl = wrapper.getImplementation();
            if (impl != null) {
                decodeParameters(parameterGenotype, impl);
            }
        }

        return wrapper;
    }

    protected final ParameterDecoder parameterDecoder;
    protected final Map<ParameterReference, ParameterReference> selectParameterRef = new HashMap<ParameterReference, ParameterReference>();
    protected final Map<ParameterReference, List<Object>> selectParametersMap = new HashMap<ParameterReference, List<Object>>();


    /**
     * Initialize selectParameterRef and selectParametersMap parameterDecoder.
     * (Copy of code from the constructor of net.sf.opendse.optimization.DesignSpaceExplorationDecoder)
     */
    protected void initParameters() {
        for (Element element : Models.getElements(spec)) {
            for (String name : element.getAttributeNames()) {
                Parameter parameter = element.getAttributeParameter(name);
                if (parameter != null && parameter instanceof ParameterSelect) {
                    ParameterSelect parameterSelect = (ParameterSelect) parameter;

                    String reference = parameterSelect.getReference();

                    if (reference != null) {
                        ParameterReference pref = new ParameterReference(element, name);
                        ParameterReference prefref = new ParameterReference(element, reference);

                        selectParameterRef.put(pref, prefref);
                        selectParametersMap.put(pref, Arrays.asList(parameterSelect.getElements()));
                    }
                }
            }
        }
    }

    /**
     * Decodes the {@link Parameter}s and sets the according {@link Attributes}.
     * (Copy of decodeParameters from net.sf.opendse.optimization.DesignSpaceExplorationDecoder)
     *
     * @param parameterGenotype
     *            the parameter genotype
     * @param implementation
     *            the corresponding implementation to augment
     */
    protected void decodeParameters(CompositeGenotype<String, Genotype> parameterGenotype,
            Specification implementation) {
        ParameterMap parameterMap = parameterDecoder.decode(parameterGenotype);

        Map<String, Element> elementMap = Models.getElementsMap(implementation);

        if (!parameterMap.isEmpty()) {
            for (Entry<ParameterReference, Object> entry : parameterMap.entrySet()) {
                ParameterReference ref = entry.getKey();
                String id = ref.getId();
                String attribute = ref.getAttribute();

                if (elementMap.containsKey(id)) {
                    Element element = elementMap.get(id);
                    Object value = entry.getValue();
                    element.setAttribute(attribute, value);
                }
            }
        }

        for (Entry<ParameterReference, ParameterReference> entry : selectParameterRef.entrySet()) {
            ParameterReference ref = entry.getKey();
            ParameterReference refref = entry.getValue();

            String id = ref.getId();
            String refAttribute = ref.getAttribute();
            String refrefAttribute = refref.getAttribute();

            Element element = elementMap.get(id);
            if (element != null) {
                Object o = element.getAttribute(refrefAttribute);
                int index = ((ParameterSelect) (refref.getParameter())).indexOf(o);

                Object o2 = selectParametersMap.get(ref).get(index);
                element.setAttribute(refAttribute, o2);
            }
        }
    }
}
